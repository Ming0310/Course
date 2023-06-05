use sakila;

CREATE VIEW customer_info AS
SELECT customer.customer_id CID,
CONCAT(customer.first_name,' ',customer.last_name) CNAME,
SUM(payment.amount) TOTAL_SPEND,
GROUP_CONCAT(CONCAT(film.title) SEPARATOR ',') film_info
FROM customer,payment,rental,inventory,film
WHERE rental.rental_id = payment.rental_id AND rental.inventory_id = inventory.inventory_id 
AND film.film_id = inventory.film_id AND customer.customer_id = rental.customer_id
GROUP BY customer.customer_id;
SELECT * FROM customer_info;

delimiter $$
CREATE TRIGGER customer_del BEFORE DELETE ON customer 
	FOR EACH ROW BEGIN
		DELETE FROM rental WHERE rental.customer_id = old.customer_id;
        DELETE FROM payment WHERE payment.customer_id = old.customer_id;
END$$

SELECT * FROM rental WHERE rental.customer_id = 598;
SELECT * FROM payment WHERE payment.customer_id = 598;
DELETE FROM customer WHERE customer_id = 598;